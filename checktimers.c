#include "checktimers.h"
#include "error.h"
#include "sqlite3.h"

void sql_prep(sqlite3 *, char *, sqlite3_stmt **);
int add_timer(sqlite3 *);
int vac_timers(sqlite3 *);
void print_wait(time_t);


void sql_prep(sqlite3 *db, char *zSql, sqlite3_stmt **query)
{
  if (sqlite3_prepare_v2(db, zSql, 512, query, NULL) != SQLITE_OK) {
    err_msg("sqlite3: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    exit(1);
  }
}


int add_timer(sqlite3 *db)
{
  int choice;
  time_t timestamp;
  char zSql[1024];
  sqlite3_stmt *toon;
  int currow;
  const unsigned char *curtoon;
  sqlite3_stmt *raid;
  const unsigned char *curraid;
  
  sql_prep(db, "select rowid,name from toons;", &toon);

  while (sqlite3_step(toon) == SQLITE_ROW) {
    currow = sqlite3_column_int(toon, 0);
    curtoon = sqlite3_column_text(toon, 1);
    printf("%d. %s\n", currow, curtoon);
  }
  sqlite3_finalize(toon);

  do {
    printf("> ");
    if (!scanf("%d", &choice))
      getchar();
  } while (choice < 1 || choice > currow);

  snprintf(zSql, 256, "select name from toons where rowid=%d;", choice);
  sql_prep(db, zSql, &toon);
  sqlite3_step(toon);
  curtoon = sqlite3_column_text(toon, 0);

  sql_prep(db, "select rowid,name from raids;", &raid);
  while (sqlite3_step(raid) == SQLITE_ROW) {
    currow = sqlite3_column_int(raid, 0);
    curraid = sqlite3_column_text(raid, 1);
    printf("%d. %s\n", currow, curraid);
  }
  sqlite3_finalize(raid);

  do {
    printf("> ");
    if (!scanf("%d", &choice))
      getchar();
  } while (choice < 1 || choice > currow);

  snprintf(zSql, 256, "select name from raids where rowid=%d;", choice);
  sql_prep(db, zSql, &raid);
  sqlite3_step(raid);
  curraid = sqlite3_column_text(raid, 0);

  timestamp = time(NULL);
  
  snprintf(zSql, 1024, "insert into timers values(\"%s\", \"%s\", %ld);",
	   curtoon, curraid, timestamp);
  if (sqlite3_exec(db, zSql, NULL, NULL, NULL))
    err_quit("sqlite3_exec error");
  
  printf("timer added for toon: %s, raid: %s\n", curtoon, curraid);
  
  return 0;
}


int vac_timers(sqlite3 *db)
{
  char zSql[1024];
  time_t cursecs;

  cursecs = time(NULL);
  snprintf(zSql, 512, "delete from timers where timestamp < %ld",
	   cursecs-TIMERSECS);

  if (sqlite3_exec(db, zSql, NULL, NULL, NULL))
    err_quit("sqlite3_exec error");
  
  return 0;
}


void print_wait(time_t secs)
{
  int days, hours, mins;

  days = (secs/3600)/24;
  secs -= days*24*3600;
  hours = secs/3600;
  secs -= hours*3600;
  mins = secs/60;

  printf("(");
  if (days)
    printf("%d days, ", days);
  if (hours)
    printf("%d hours, ", hours);
  printf("%d mins)\n", mins);
}


int main(int argc, char *argv[])
{
  sqlite3 *db;
  char zSql[1024];
  sqlite3_stmt *toon;
  const unsigned char *curtoon;
  sqlite3_stmt *timer;
  time_t secs, cursecs;
  struct tm *tm;
  char endtime[256];
  int loop;

  if (sqlite3_open(DBNAME, &db)) {
    err_msg("sqlite3 open failed: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 1;
  }

  if (!strncmp(basename(argv[0]), "addtimer", 8))
    return add_timer(db);
  
  if (!strncmp(basename(argv[0]), "vactimers", 11))
    return vac_timers(db);

  sql_prep(db, "select name from toons;", &toon);
  while (sqlite3_step(toon) == SQLITE_ROW) {
    loop = 1;
    curtoon = sqlite3_column_text(toon, 0);

    snprintf(zSql, 256, "select raid,timestamp from timers where name=\'%s\';",
	     curtoon);
    sql_prep(db, zSql, &timer);
    cursecs = time(NULL);
    while (sqlite3_step(timer) == SQLITE_ROW) {
      secs = sqlite3_column_int64(timer, 1) + TIMERSECS;
      if (secs < cursecs)
	continue;
      if (loop) {
	printf("%s\n", curtoon);
	loop = 0;
      }
      tm = localtime(&secs);
      strftime(endtime, 256, "%a %m/%d %X", tm);
      printf("\t%-25s %s ", sqlite3_column_text(timer,0), endtime);
      print_wait(secs-cursecs);
    }
    sqlite3_finalize(timer);
  }

  sqlite3_finalize(toon);
  sqlite3_close(db);

  return 0;
}

#!/bin/bash
# Copyright 2008-2012 Eric MAEKER

MYSQL_PATH=""
OUTPUT_FILE=""
MYSQL_ROOT_PASS=""
MYSQL=""
MYSQL_ADMIN=""

SCRIPT_NAME=`basename $0`
if [[ "`echo $0 | cut -c1`" = "/" ]]; then
  SCRIPT_PATH=`dirname $0`
else
  SCRIPT_PATH=`pwd`/`echo $0 | sed -e s/$SCRIPT_NAME//`
fi

# Get OS name
sys=`uname -s`
if [[ "$sys" == "Linux" ]] ; then
    echo "Running under Linux"
elif [[ "$sys" == "Darwin" ]] ; then
    echo "Running under MacOs"
    MYSQL_PATH="/usr/local/mysql/bin"
    echo "Setting MySQL path to: "$MYSQL_PATH
fi

# Prepare output file
OUTPUT_FILE=$SCRIPT_PATH"/mysqldebug.txt"
COMMANDS_FILE=$SCRIPT_PATH"/commands.sql"

showHelp()
{
  echo $SCRIPT_NAME" helps devs to debug MySQL configuration problems."
  echo "This script is part of the FreeMedForms project."
  echo "Usage : $SCRIPT_NAME -p rootpassword"
  echo "Options :"
  echo " -p  define the mysql root password"
  echo " -h  show this help"
  echo
}

checkMySQLServer()
{
  # MySQL running ?
  echo "## TESTING MYSQL SERVER" >> $OUTPUT_FILE
  echo "*** Testing MySQL server status"
  if [[ "$sys" == "Linux" ]] ; then
    echo `/etc/init.d/mysqld status` >> $OUTPUT_FILE
    echo "    "`/etc/init.d/mysqld status`
  elif [[ "$sys" == "Darwin" ]] ; then
    echo "*** System Root password?"
    echo `sudo $MYSQL_PATH/../support-files/mysql.server status` >> $OUTPUT_FILE
    echo "    "`sudo $MYSQL_PATH/../support-files/mysql.server status`
  fi
}

createSqlCommandFile()
{
   clearSqlCommandFile
   echo "SELECT '### ROOT GRANTS';\n" > $COMMANDS_FILE
   echo "SHOW GRANTS FOR 'root'@'localhost';\n" >> $COMMANDS_FILE

   echo "SELECT ' ';\n" >> $COMMANDS_FILE
   echo "SELECT '### FMF_ADMIN GRANTS';\n" >> $COMMANDS_FILE
   echo "SHOW GRANTS FOR 'fmf_admin'@'localhost';\n" >> $COMMANDS_FILE

   echo "SELECT ' ';\n" >> $COMMANDS_FILE
   echo "SELECT '### LOCALHOST ANONYMOUS USER';\n" >> $COMMANDS_FILE
   echo "SELECT mysql.user.Host, mysql.user.User, mysql.user.Password FROM mysql.user WHERE (mysql.user.User is null and mysql.user.Host='localhost');\n" >> $COMMANDS_FILE

   echo "SELECT ' ';\n" >> $COMMANDS_FILE
   echo "SELECT '### FMF_ DATABASES';\n" >> $COMMANDS_FILE
   echo "SHOW DATABASES LIKE 'fmf_%';\n" >> $COMMANDS_FILE

   echo "SELECT ' ';\n" >> $COMMANDS_FILE
   echo "SELECT '### ALL DATABASES';\n" >> $COMMANDS_FILE
   echo "SHOW DATABASES;\n" >> $COMMANDS_FILE

   echo "SELECT ' ';\n" >> $COMMANDS_FILE
   echo "SELECT '### FMF_USERS';\n" >> $COMMANDS_FILE
   echo "SELECT fmf_users.users.NAME, fmf_users.users.SURNAME, fmf_users.users.USER_UUID, fmf_users.users.ISVIRTUAL FROM fmf_users.users WHERE fmf_users.users.VALIDITY=1;\n" >> $COMMANDS_FILE

   echo "SELECT ' ';\n" >> $COMMANDS_FILE
   echo "SELECT '### MYSQL USERS FOR FMF';\n" >> $COMMANDS_FILE
   echo "SELECT DISTINCT mysql.user.User, mysql.user.Host FROM mysql.db" >> $COMMANDS_FILE
   echo "JOIN mysql.user on mysql.user.User=mysql.db.User" >> $COMMANDS_FILE
   echo "WHERE mysql.db.Db='fmf\_%';" >> $COMMANDS_FILE
}

clearSqlCommandFile()
{
   if [[ -e $COMMAND_FILE ]]; then
      rm -r $COMMANDS_FILE
   fi
}

extractData()
{
  echo "*** Starting data extraction"
  echo "## MySQL CONFIGURATION \n" >> $OUTPUT_FILE
  echo `$MYSQL_ADMIN ping` >> $OUTPUT_FILE
  echo " " >> $OUTPUT_FILE

  echo "`$MYSQL_ADMIN status`" >> $OUTPUT_FILE
  echo " " >> $OUTPUT_FILE

  echo "`$MYSQL_ADMIN extended-status`" >> $OUTPUT_FILE
  echo " " >> $OUTPUT_FILE

  echo "`$MYSQL_ADMIN variables`" >> $OUTPUT_FILE
  echo " " >> $OUTPUT_FILE

  echo "## DATA EXTRACTION\n" >> $OUTPUT_FILE
  # Extract user table
  $MYSQL mysql < $COMMANDS_FILE >> $OUTPUT_FILE
  echo "    Done"
}

# Parse options
while getopts "p:h" option
do
        case $option in
                p) MYSQL_ROOT_PASS="-p"$OPTARG" ";
                ;;
                h) showHelp
                    exit 0
                ;;
        esac
done

MYSQL=$MYSQL_PATH"/mysql -uroot "$MYSQL_ROOT_PASS
MYSQL_ADMIN=$MYSQL_PATH"/mysqladmin"

echo "*** Starting MySQL debugging script at: "`date`" on "`hostname` > $OUTPUT_FILE

echo
checkMySQLServer
createSqlCommandFile
extractData
clearSqlCommandFile

echo "*** Content extracted into file: "$OUTPUT_FILE
read -n1 -p "View result now? [y/n]"
if [[ $REPLY = [yY] ]]; then
   echo
   more $OUTPUT_FILE
fi

exit 0

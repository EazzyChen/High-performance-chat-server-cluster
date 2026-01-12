aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./middleware MIDDLEWARE_LIST)

add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MIDDLEWARE_LIST})

target_link_libraries(ChatServer muduo_net muduo_base hiredis mysqlclient pthread)
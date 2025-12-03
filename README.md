# Exam_rank_06
Write a program that will listen for client to connect on a certain port on 127.0.0.1 and will let clients to speak with each other

### Attention


``` mermaid
flowchart TD
    A[Socket<br>bind<br>listen] -->|max_fd = sockfd| B[FD_ZERO &actve_set<br>FD_SET sockfd &active_set<br>bzero clients]
    B --> C{while True} 
    C -->|false| Z[end]
    C -->|sets inizializacion<br> read_set = write_set = active_set | D{select < 0<br>there are socket to read}
    D -->|true| C
    D -->|False| F{for<br> df <= fd_max }
    F --> |for's end| C 
    F --> G{!FD_ISSET fd &read_set}
    G --> |True| C
    G --> H{fd == sockfd} --> |yes, new conexion|I[clients fd<br> FD_SET &active_set]
    H --> |No , existing connection|J[receive data]
    J --> K{ r< 0}
    K --> |True, cliente left| X[FD_CLR <br>]
    K --> |False, extract messages| P[prefix<br> full]
    X --> C
    P --> C
  ```

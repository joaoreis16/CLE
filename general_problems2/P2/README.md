## Anotations

### **MPI_Reduce**

```C
MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
```

Essa linha de código está em C e é uma chamada a função MPI_Reduce, que é uma função da biblioteca MPI (Message Passing Interface) utilizada para realizar uma operação de redução em um conjunto de dados distribuídos em um programa paralelo que utiliza MPI para comunicação entre processos.

Aqui está uma explicação dos argumentos dessa chamada de função:

 * **&local_min**: É um ponteiro para a variável local_min, que é o valor local que cada processo contribui para a operação de redução. É o valor que será reduzido e combinado com outros valores em todos os processos.

 * **&global_min**: É um ponteiro para a variável global_min, que é o resultado final da operação de redução. O valor reduzido de todos os processos será armazenado nesta variável no processo raiz (processo com rank 0) após a operação de redução.

 * **1**: É o número de elementos que serão reduzidos. Neste caso, é 1, pois estamos reduzindo apenas uma variável (local_min).

 * **MPI_INT**: É o tipo de dado do elemento que será reduzido. Neste caso, é um inteiro (int).

 * **MPI_MIN**: É o operador de redução que será aplicado aos elementos. Neste caso, é o operador de mínimo, que irá encontrar o valor mínimo entre todos os valores locais dos processos.

 * **0**: É o rank do processo raiz (root) que irá receber o resultado da operação de redução. Neste caso, é o processo com rank 0.

 * **MPI_COMM_WORLD**: É o comunicador que define o grupo de processos envolvidos na operação de redução. Neste caso, é o comunicador MPI_COMM_WORLD, que é um comunicador predefinido que inclui todos os processos em execução no programa MPI.



### **MPI_Scatter**

```C
MPI_Scatter(sendData, 4, MPI_INT, recvData, 4, MPI_INT, 0, MPI_COMM_WORLD);
```

Essa linha de código faz uso da função MPI_Scatter da biblioteca MPI para distribuir dados de um processo raiz (root) para os demais processos em um comunicador MPI_COMM_WORLD. Vou explicar os principais argumentos dessa função:

 * **sendData**: É o endereço de memória onde os dados a serem enviados estão armazenados no processo raiz (root). No exemplo dado, espera-se que sendData seja um ponteiro para um array de inteiros com pelo menos 4 elementos.

 * **4**: É o número de elementos a serem enviados para cada processo. Neste caso, é esperado que cada processo receba 4 elementos do array sendData.

 * **MPI_INT**: É o tipo de dado dos elementos no array sendData e recvData. Neste caso, são inteiros.

 * **recvData**: É o endereço de memória onde os dados recebidos serão armazenados em cada processo. No exemplo dado, espera-se que recvData seja um ponteiro para um array de inteiros com pelo menos 4 elementos, que será preenchido com os dados recebidos pelo MPI_Scatter.

 * **4**: É o número de elementos a serem recebidos por cada processo. Neste caso, é esperado que cada processo receba 4 elementos do processo raiz.

 * **MPI_INT**: É o tipo de dado dos elementos no array recvData. Neste caso, são inteiros.

 * **0**: É o rank (identificador) do processo raiz que está enviando os dados. Neste caso, o processo com rank 0 é o processo raiz.

 * **MPI_COMM_WORLD**: É o comunicador que define o grupo de processos participantes na operação de comunicação. Neste caso, é o comunicador predefinido MPI_COMM_WORLD, que inclui todos os processos em execução no programa MPI.

Em resumo, essa linha de código distribui 4 elementos de um array de inteiros chamado sendData do processo raiz (com rank 0) para os demais processos no comunicador MPI_COMM_WORLD, armazenando os dados recebidos em um array de inteiros chamado recvData em cada processo. Essa operação é realizada de forma coletiva, ou seja, todos os processos participantes executam essa função simultaneamente.
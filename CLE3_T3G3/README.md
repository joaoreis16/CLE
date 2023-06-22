## 3rd assignment CLE: CUDA

grade: 12

### máquina virtual

máquina: banana.ua.pt
login: cle33
pass:  +yi62Mppta


### entrar na máquina
ssh cle33@banana.ua.pt

(crtl + D para sair)


### colocar CodeSamples.zip na máquina (temos que estar fora da máquina virtual banana)
sftp cle33@banana.ua.pt
put CodeSamples.zip
bye

cd 4ano/CLE/CLE3_T3G3/prog1/
sftp cle33@banana.ua.pt
put main.cu
bye

### complilar e correr o projeto
make

./main
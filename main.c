// Projeto Aula - 04
// Gabriel Sanches Pinto
// Cauan Gabriel de Souza

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define MAX 20

struct bufRegistro_ins
// Guarda os registros do inserir.bin
{
    char id_aluno[4];
    char sigla_disc[4];
    char nome_aluno[50];
    char nome_disc[50];
    float media;
    float freq;
};

struct Chave
// Guarda as chaves (id_aluno e sigla_disciplina)
{
    char id_aluno[4];
    char sigla_disc[4];
};

struct Cabecalho
// Guarda o header do arquivo binario
{
    int regLidos_insere;
    int regLidos_remove;
    int BOF_dispo;
};

bool comparaChaves(char *bufRegistro, char *bufChaves)
// Compara chaves recebidas
{

    if (bufRegistro[0] == '*')
    // Testa se o registro ja nao foi marcado como removido
    {
        return false;
    }
    for (int i = 0; i < strlen(bufChaves); i++)
    {
        if (bufRegistro[i] != bufChaves[i])
        // Checa a igualdade de cada char
        {
            return false;
        }
    }
    return true;
}

void inserir(int index)
// Insere registros do insere.bin no arquivo saida.bin
{

    // Abertura dos arquivos
    FILE *in;
    FILE *out;

    if ((in = fopen("insere.bin", "r+b")) == NULL)
    {
        printf("\nNao foi possivel abrir o arquivo insere");
        return;
    }

    if ((out = fopen("saida.bin", "r+b")) == NULL)
    {
        printf("\nErro ao abrir o arquivo");
        return;
    }

    struct bufRegistro_ins registro;
    struct Chave key;
    struct Cabecalho header;
    int tam_reg = 0;
    char chave_inserir[8];
    char chave_comparar[8];
    char buffer_comparar[256];
    char buffer[512];

    rewind(out);

    // Leitura do cabecalho do arquivo saida.bin
    fread(&header, sizeof(struct Cabecalho), 1, out);

    // Procura o registro referente ao index escolhido
    fseek(in, (index - 1) * sizeof(registro), SEEK_SET);

    if (fgetc(in) == EOF)
    // Testa se o valor do indice nao apontou para um indice inexistente
    {
        printf("\nEsse indice nao esta disponivel\n");
        return;
    }

    // Retorna 1 byte da posicao atual
    fseek(in, -1, SEEK_CUR);

    // Le o registro que sera inserido
    fread(&registro, sizeof(registro), 1, in);

    // Insere os dados lidos em um buffer formatado para inserido no arquivo saida.bin
    sprintf(buffer, "%s#%s#%s#%s#%.2f#%.2f", registro.id_aluno, registro.sigla_disc, registro.nome_aluno, registro.nome_disc, registro.media, registro.freq);

    // Coleta as chaves do registro para ser comparada com as chaves ja existentes
    sprintf(chave_inserir, "%s#%s", registro.id_aluno, registro.sigla_disc);

    // Procura o registro referente ao index escolhido
    fseek(in, (index - 1) * sizeof(key), SEEK_SET);

    while (true)
    // Loop que ira testar se ja existe um registro com a mesma chave no arquivo
    {
        // Le o tamanho do registro no arquivo saida.bin
        fread(&tam_reg, sizeof(int), 1, out);
        // Le o registro do arquivo e armazena em um buffer para ser comparado
        fread(&buffer_comparar, tam_reg, 1, out);
        // Coleta os 7 primeiros bytes (000#AAA) que compoem a chave
        strncpy(chave_comparar, buffer_comparar, 7);
        if (!comparaChaves(chave_comparar, chave_inserir))
        // Testa se a chave nova ja nao tem um registro correspondente no arquivo de saida.bin
        {
            // Avanca 1 caractere
            fgetc(out);
            if (feof(out))
            // Testa se chegou no final do arquivo (chave nao encontrada)
            {
                break;
            }
            else
            {
                fseek(out, -1, SEEK_CUR);
            }
        }
        else
        {
            printf("\nEsse registro ja existe!");
            return;
        }
    }
    tam_reg = strlen(buffer);
    int tam_espaco;

    // Controle de espacos disponiveis no arquivo
    int disponivel_anterior = 8;
    int disponivel_atual;
    int disponivel_posterior;

    if (header.BOF_dispo != -1)
    // Caso seja -1, nao ha espacos disponiveis
    {
        fseek(out, header.BOF_dispo, SEEK_SET);
        disponivel_atual = header.BOF_dispo;

        while (true)
        // Move-se dentro dos espacos disponiveis
        {
            fread(&tam_espaco, sizeof(int), 1, out);
            fgetc(out);
            // Le o proximo espaco disponivel
            fread(&disponivel_posterior, sizeof(int), 1, out);

            if (tam_reg <= tam_espaco)
            // Verifica se o registro cabe no espaco disponivel
            {
                // Escreve o registro
                fseek(out, (-1) * sizeof(int) - 1, SEEK_CUR);
                fwrite(buffer, 1, tam_reg, out);
                printf("inserido registro entre os arquivos");

                if (disponivel_anterior == 8)
                // Atualiza o cabecalho de acordo com a nova lista de espacos disponiveis
                {
                    header.BOF_dispo = disponivel_posterior;
                    rewind(out);
                    fwrite(&header, sizeof(struct Cabecalho), 1, out);
                }
                else
                {
                    fseek(out, disponivel_anterior + sizeof(int) + 1, SEEK_SET);
                    fwrite(&disponivel_posterior, 1, sizeof(int), out);
                }

                // Incrementa os registros lido no cabecalho
                header.regLidos_insere++;
                break;
            }
            else
            {
                if (disponivel_posterior == -1)
                // Atualiza os offsets
                {
                    fseek(out, 0, SEEK_END);
                    fwrite(&tam_reg, sizeof(int), 1, out);
                    fwrite(buffer, tam_reg, 1, out);
                    header.regLidos_insere++;
                    break;
                }
                else
                {
                    if (disponivel_anterior == 8)
                    {
                        header.BOF_dispo = disponivel_atual;
                        rewind(out);
                        fwrite(&header, sizeof(struct Cabecalho), 1, out);
                    }
                    else
                    {
                        fseek(out, disponivel_anterior + sizeof(int) + 1, SEEK_SET);
                        fwrite(&disponivel_atual, 1, sizeof(int), out);
                    }

                    fseek(out, disponivel_atual + sizeof(int) + 1, SEEK_SET);
                    fwrite(&disponivel_posterior, sizeof(int), 1, out);

                    disponivel_anterior = disponivel_atual;
                    disponivel_atual = disponivel_posterior;
                    fseek(out, disponivel_posterior, SEEK_SET);
                }
            }
        }
    }
    else
    // Anexa o registro no final do arquivo quando nao ha espacos diponiveis
    {
        fseek(out, 0, SEEK_END);
        fwrite(&tam_reg, sizeof(int), 1, out);
        fwrite(buffer, tam_reg, 1, out);
        header.regLidos_insere++;
    }

    rewind(out);
    fwrite(&header, sizeof(struct Cabecalho), 1, out);
    printf("\nRegistro incluido com sucesso!\n");

    fclose(in);
    fclose(out);
}

void remover(int index)
// Remove com base em uma chave do arquivo remove.bin, seu respectivo registro em saida.bin
{

    // Abertura dos arquivos
    FILE *in;
    FILE *out;

    if ((in = fopen("remove.bin", "r+b")) == NULL)
    {
        printf("Nao foi possivel abrir o arquivo");
        return;
    }

    if ((out = fopen("saida.bin", "r+b")) == NULL)
    {
        printf("Erro ao abrir o arquivo\n");
        return;
    }

    struct Chave key;
    struct Cabecalho header;

    int tam_reg = 0;
    int acum_BOF = sizeof(struct Cabecalho);
    int reg_checados = 0;
    char buffer_key[256];
    char buffer_registro[512];

    rewind(out);
    // Leitura do cabecalho do arquivo saida.bin
    fread(&header, sizeof(struct Cabecalho), 1, out);
    // Procura o registro referente ao index escolhido
    fseek(in, (index - 1) * sizeof(key), SEEK_SET);

    if (fgetc(in) == EOF)
    // Testa se o valor do indice nao apontou para um indice inexistente
    {
        printf("\nNao ha essa remocao\n");
        return;
    }

    // Retorna 1 byte da posicao atual
    fseek(in, -1, SEEK_CUR);

    // Le o registro que sera removido
    fread(&key, sizeof(key), 1, in);

    // Insere em um buffer a chave que sera buscada
    sprintf(buffer_key, "%s#%s", key.id_aluno, key.sigla_disc);

    while (true)
    // Loop que ira testar se existe o registro que deve ser excluido
    {
        // Le o tamanho do registro no arquivo saida.bin
        fread(&tam_reg, sizeof(int), 1, out);
        // Le o registro do arquivo e armazena em um buffer para ser comparado
        fread(&buffer_registro, tam_reg, 1, out);

        if (!comparaChaves(buffer_registro, buffer_key)) // Ta dando erro aqui na comparação dos registros, o codigo deveria pedir a tag
        // Compara chaves tem que ser false para entrar no loop
        // Testa se a chave existe no arquivo de saida.bin
        {
            // Incrementa o contador de bytes percorridos
            acum_BOF += tam_reg;
            reg_checados++;

            fgetc(out);
            if (feof(out))
            {
                // Incrementa o contador de registros removidos do cabecalho
                header.regLidos_remove++;

                rewind(out);
                // Atualiza o cabecalho
                fwrite(&header, sizeof(struct Cabecalho), 1, out);

                printf("\nNao ha nenhum registro correspondente a essa chave no arquivo\n");

                fclose(in);

                return;
            }

            else
            {
                fseek(out, -1, SEEK_CUR);
            }
        }

        else
        {
            // Sai do loop ao encontrar a chave selecionada
            break;
        }
    }

    fseek(out, (-1) * tam_reg, SEEK_CUR);
    char estrela = '*';

    // Marca o registro como removido
    fwrite(&estrela, 1, 1, out);

    // Imprime o offset no cabecalho do arquivo
    fwrite(&header.BOF_dispo, sizeof(int), 1, out);

    // Atualiza o cabecalho
    acum_BOF += reg_checados * sizeof(int);
    header.BOF_dispo = acum_BOF;
    header.regLidos_remove++;

    rewind(out);

    printf("O que sera escrito no cabecalho: %d, %d, %d", header.BOF_dispo, header.regLidos_insere, header.regLidos_remove);
    fwrite(&header, sizeof(struct Cabecalho), 1, out); // Escreve o cabeçalho atualizado no arquivo de registros
    // opcao 1, o header nao esta sendo escrito da forma correta

    printf("\nRegistro excluido com sucesso!\n");
    fclose(out);
    fclose(in);
}

bool carregaArquivos()
// Recria os arquivos saida.bin e inserir.bin (fornecido para execucao da atividade)
{
    FILE *fd;
    
    //////////////////////////////
    struct hist {
        char id_aluno[4];
        char sigla_disc[4];
        char nome_aluno[50];
        char nome_disc[50];
        float media;
        float freq;
    } vet[8] = {{"001", "111", "Joao da Silva", "Disc-111", 7.5, 72.3},
                {"001", "222", "Joao da Silva", "Disc-222", 8.2, 80.4},
                {"001", "333", "Joao da Silva", "Disc-333", 5.4, 79.7},                
                {"002", "444", "Paulo Souza", "Disc-444", 6.8, 91.3},
            
                {"003", "555", "Pedro", "Disc-555", 6.3, 72.7},
                {"001", "666", "Joao da Silva", "Disc-666", 8.3, 77.7},
                {"003", "777", "Pedro", "Disc-777", 9.7, 89.7},
                
                {"005", "888", "Lucas Silva e Silva", "Disc-888-888", 9.5, 92.7}};
       
    fd = fopen("insere.bin", "w+b");
    fwrite(vet, sizeof(vet), 1, fd);
    fclose(fd);
    
    //////////////////////////////
	struct remove {
        char id_aluno[4];
        char sigla_disc[4];
    } vet_r[4] = {{"001","333"},
                  {"002","444"},
                  
                  {"003","777"},
                  {"001","111"}};
       
    fd = fopen("remove.bin", "w+b");
    fwrite(vet_r, sizeof(vet_r), 1, fd);
    fclose(fd);
    return true;
}

void compactar()
{
    int tam_espaco, tam_registro;
    char buffer[512];
    char ch_aux;
    int i;
    int line_counter;

    struct Cabecalho header;

    FILE *out;
    FILE *fp_aux;
    
    // Abre o arquivo a ser compactado e um arquivo auxiliar
    if ((fp_aux = fopen("auxiliar.bin", "w+b")) == NULL)
    {
        printf("\nNao foi possivel abrir o arquivo");
        return;
    }

    if ((out = fopen("saida.bin", "r+b")) == NULL)
    {
        printf("\nErro ao abrir o arquivo\n");
        return;
    }

    // Volta ao inicio do arquivo e le o header do arquivo a ser compactado
    rewind(out);
    fread(&header, sizeof(struct Cabecalho), 1, out);
    header.BOF_dispo = -1;
    //Atualiza o BOF header ja que agora o nao havera mais um registro disponivel no meio do arquivo 
    
    //Escreve o header atualizado no arquivo auxiliar
    fwrite(&header, sizeof(struct Cabecalho), 1, fp_aux);

    //Checa os registro do arquivo buscando aqueles que não estejam marcados como removidos
    while (true)
    {
        //Pega o primeiro char para verificar se o arquivo esta marcado como removido
        fread(&tam_espaco, sizeof(int), 1, out);
        ch_aux = fgetc(out);

        if (ch_aux == '*')
        {

            fseek(out, (tam_espaco - 1), SEEK_CUR);
            continue; //Se o registro foi marcado como removido ele ignora esse registro e volta para o começo do while
        }

        if (ch_aux == EOF) //Checa se chegou até o final do arquivo, quando isso acontece sai do loop com um break
        {
            break;
        }

        fseek(out, -1, SEEK_CUR);

        line_counter = 0;
        tam_registro = 0;
        i = 0;

        while (line_counter < 5) //Itera pelo registro character a character obtendo o tamanho do registro com um contador
        {
            buffer[i] = fgetc(out); //Tambem copia o registro do arquivo a ser compactado pro buffer
            if (buffer[i] == '#')
                line_counter++;
            tam_registro++;
            i++;
        }
        int j = i;
        while(j<i+4){
            printf("\nESTOU NO FOR");
            buffer[j] = fgetc(out);
            tam_registro++;
            j++;
        }
        
        // Coloca \0 no final do buffer
        buffer[j] = '\0';
        printf("buffer por enquanto = %s", buffer);

        //Escreve o tamanho do registro no inicio do registro já no arquivo auxiliar 
        fwrite(&tam_registro, sizeof(int), 1, fp_aux);

        //Escreve o buffer com o conteudo do registro encontrado como nao removido no arquivo auxiliar
        fwrite(buffer, strlen(buffer), 1, fp_aux);

        fseek(out, tam_espaco - tam_registro, SEEK_CUR);
        ch_aux = fgetc(out);

        if (ch_aux != EOF) //Verifica se nao chegou no final do arquivo a ser compactado
        {

            fseek(out, -1, SEEK_CUR);
        }
        else
        {

            break; //Se for o final do arquivo, sai do loop
        }
    }

    fclose(out);
    fclose(fp_aux);

    //Agora Vamos substituir o arquivo auxiliar, que nesse momento contem apenas os registro nao excluidos do arquivo de saida
    //Pelo proprio arquivo da saida

    char arquivo_original[100] = "saida.bin";
    char arquivo_auxiliar[100] = "auxiliar.bin";
    
    //Fazemos isso trocando os nomes desses arquivo e depois excluimos o arquivo de saida original
    if (remove(arquivo_original) == -1)
        printf("\nErro ao excluir arquivo\n");
    if (rename(arquivo_auxiliar, arquivo_original) != 0)
        printf("\nErro ao renomear arquivo\n");
    remove(arquivo_auxiliar);

    printf("\nArquivo compactado com sucesso!\n");

    if ((out = fopen("saida.bin", "r+b")) == NULL)
    {
        printf("\nNao foi possivel abrir o arquivo");
        return;
    }
    fclose(out);
}

int main()
{
    int operacaoDesejada = 0;

    FILE *out = NULL;
    int index;

    // Cabecalho
    struct Cabecalho header;
    header.regLidos_insere = 0;
    header.regLidos_remove = 0;
    header.BOF_dispo = -1;

    if ((out = fopen("saida.bin", "r+b")) == NULL)
    {
        if ((out = fopen("saida.bin", "w+b")) != NULL)
        {
            fwrite(&header, sizeof(struct Cabecalho), 1, out);
            rewind(out);
        }
        else
        {
            printf("\nFalha ao ler/criar arquivo saida.bin.");
            return 0;
        }
    }

    fread(&header, sizeof(struct Cabecalho), 1, out);

    while (true)
    {
        // Menu principal
        printf("\n------------------------------\n");
        printf("Digite o numero correspondente a operacao desejada: \n1 - Insercao\n2 - Remocao\n3 - Compactacao\n4 - Encerrar programa\n");
        printf("------------------------------\n");

        scanf("%d", &operacaoDesejada);

        if (operacaoDesejada != 1 && operacaoDesejada != 2 && operacaoDesejada != 3 && operacaoDesejada != 4 && operacaoDesejada != 5 && operacaoDesejada != 6)
        {
            printf("\nFormato invalido, verifique o numero correspondente a opcao desejada e tente novamente:\n");
        }
        if (operacaoDesejada == 1)
        {
            printf("\nindex: ");
            scanf("%d", &index);
            inserir(index);
        }
        if (operacaoDesejada == 2)
        {
            printf("\nindex: ");
            scanf("%d", &index);
            remover(index);
        }
        if (operacaoDesejada == 3)
        {
            compactar();
        }
        if (operacaoDesejada == 5)
        {
            if (carregaArquivos())
            {
                printf("\nArquivos carregados com sucesso!\n");
            }
            else
            {
                printf("\nErro ao carregar os arquivos :(\n");
            }
        }
        if (operacaoDesejada == 4)
        {
            printf("\nEncerrando o progama...");
            fclose(out);
            return 0;
        }
    }
}
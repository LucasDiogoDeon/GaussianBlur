#define _CRT_SECURE_NO_DEPRECATE
#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "omp.h"





// struct que armazena as cores dos pixels em RGB
struct pixel
{
	unsigned char red;   // R vermelho
	unsigned char green; // G verde
	unsigned char blue;  // B azul 
};





// Retorna o índice do pixel
// i            : linha atual
// j            : coluna atual
// image_width  : largura da imagem
// image_height : altura da imagem
int get_pixel_index(int i, int j, int image_width, int image_height)
{
	if (i < 0)
	{
		i = 0;
	}
	if (i >= image_height)
	{
		i = image_height - 1;
	}
	if (j < 0)
	{
		j = 0;
	}
	if (j >= image_width)
	{
		j = image_width - 1;
	}

	return ( i * image_width ) + j;
}





int _tmain(int argc, _TCHAR* argv[])
{
	// Declaração das variáveis

	// Tempo de leitura
	double reading_time_start; // Início
	double reading_time_end;   // Fim
	double reading_time_total; // Total

	// Tempo de processamento
	double processing_time_start; // Início
	double processing_time_end;   // Fim
	double processing_time_total; // Total

	// Tempo de escrita
	double writing_time_start; // Início
	double writing_time_end;   // Fim
	double writing_time_total; // Total

	// Tempo total
	double total_time; // Total

	char* input_file_path;  // Caminho completo do arquivo original
	char* output_file_path; // Caminho completo do arquivo a ser criado

	FILE *input_file_handle;
	FILE *output_file_handle;
	
	int image_width;  // Largura, em pixels, da imagem
	int image_height; // Altura, em pixels, da imagem
	int image_depth;  // Profundidade da imagem (fixo = 255)

	int image_pixel_count; // Quantidade total de pixels
	// / Declaração das variáveis





	// Quantidade de threads
	int thread_count; 
	printf("Quantidade de threads: ");
	scanf("%d", &thread_count);
	// / Quantidade de threads





	// Configuração do efeito blur
	/*
		// Matlab
		
		x = linspace(-3,3,31);
		y = x';               
		[X,Y] = meshgrid(x,y);
		z = exp(-(X.^2+Y.^2)/2);
		surf(x,y,z)

		// / Matlab
	*/
	const int BLUR_SIZE = 31; // Tamanho do blur
	double blur_weights[BLUR_SIZE][BLUR_SIZE]; // Pesos dos pixels
	{
		double blur_distribution[BLUR_SIZE][BLUR_SIZE] = { // Distribuição dos pesos
			0.0001, 0.0002, 0.0004, 0.0006, 0.0010, 0.0015, 0.0022, 0.0031, 0.0042, 0.0054, 0.0067, 0.0081, 0.0093, 0.0103, 0.0109, 0.0111, 0.0109, 0.0103, 0.0093, 0.0081, 0.0067, 0.0054, 0.0042, 0.0031, 0.0022, 0.0015, 0.0010, 0.0006, 0.0004, 0.0002, 0.0001,
			0.0002, 0.0004, 0.0007, 0.0011, 0.0018, 0.0027, 0.0039, 0.0055, 0.0074, 0.0097, 0.0120, 0.0144, 0.0166, 0.0183, 0.0194, 0.0198, 0.0194, 0.0183, 0.0166, 0.0144, 0.0120, 0.0097, 0.0074, 0.0055, 0.0039, 0.0027, 0.0018, 0.0011, 0.0007, 0.0004, 0.0002,
			0.0004, 0.0007, 0.0012, 0.0019, 0.0030, 0.0046, 0.0067, 0.0095, 0.0128, 0.0166, 0.0207, 0.0247, 0.0284, 0.0314, 0.0334, 0.0340, 0.0334, 0.0314, 0.0284, 0.0247, 0.0207, 0.0166, 0.0128, 0.0095, 0.0067, 0.0046, 0.0030, 0.0019, 0.0012, 0.0007, 0.0004,
			0.0006, 0.0011, 0.0019, 0.0032, 0.0050, 0.0076, 0.0111, 0.0156, 0.0211, 0.0273, 0.0340, 0.0408, 0.0469, 0.0518, 0.0550, 0.0561, 0.0550, 0.0518, 0.0469, 0.0408, 0.0340, 0.0273, 0.0211, 0.0156, 0.0111, 0.0076, 0.0050, 0.0032, 0.0019, 0.0011, 0.0006,
			0.0010, 0.0018, 0.0030, 0.0050, 0.0079, 0.0120, 0.0176, 0.0247, 0.0334, 0.0433, 0.0539, 0.0646, 0.0743, 0.0821, 0.0872, 0.0889, 0.0872, 0.0821, 0.0743, 0.0646, 0.0539, 0.0433, 0.0334, 0.0247, 0.0176, 0.0120, 0.0079, 0.0050, 0.0030, 0.0018, 0.0010,
			0.0015, 0.0027, 0.0046, 0.0076, 0.0120, 0.0183, 0.0268, 0.0376, 0.0508, 0.0659, 0.0821, 0.0983, 0.1130, 0.1249, 0.1327, 0.1353, 0.1327, 0.1249, 0.1130, 0.0983, 0.0821, 0.0659, 0.0508, 0.0376, 0.0268, 0.0183, 0.0120, 0.0076, 0.0046, 0.0027, 0.0015,
			0.0022, 0.0039, 0.0067, 0.0111, 0.0176, 0.0268, 0.0392, 0.0550, 0.0743, 0.0963, 0.1200, 0.1437, 0.1653, 0.1827, 0.1940, 0.1979, 0.1940, 0.1827, 0.1653, 0.1437, 0.1200, 0.0963, 0.0743, 0.0550, 0.0392, 0.0268, 0.0176, 0.0111, 0.0067, 0.0039, 0.0022,
			0.0031, 0.0055, 0.0095, 0.0156, 0.0247, 0.0376, 0.0550, 0.0773, 0.1044, 0.1353, 0.1686, 0.2019, 0.2322, 0.2567, 0.2725, 0.2780, 0.2725, 0.2567, 0.2322, 0.2019, 0.1686, 0.1353, 0.1044, 0.0773, 0.0550, 0.0376, 0.0247, 0.0156, 0.0095, 0.0055, 0.0031,
			0.0042, 0.0074, 0.0128, 0.0211, 0.0334, 0.0508, 0.0743, 0.1044, 0.1409, 0.1827, 0.2276, 0.2725, 0.3135, 0.3465, 0.3679, 0.3753, 0.3679, 0.3465, 0.3135, 0.2725, 0.2276, 0.1827, 0.1409, 0.1044, 0.0743, 0.0508, 0.0334, 0.0211, 0.0128, 0.0074, 0.0042,
			0.0054, 0.0097, 0.0166, 0.0273, 0.0433, 0.0659, 0.0963, 0.1353, 0.1827, 0.2369, 0.2952, 0.3535, 0.4066, 0.4493, 0.4771, 0.4868, 0.4771, 0.4493, 0.4066, 0.3535, 0.2952, 0.2369, 0.1827, 0.1353, 0.0963, 0.0659, 0.0433, 0.0273, 0.0166, 0.0097, 0.0054,
			0.0067, 0.0120, 0.0207, 0.0340, 0.0539, 0.0821, 0.1200, 0.1686, 0.2276, 0.2952, 0.3679, 0.4404, 0.5066, 0.5599, 0.5945, 0.6065, 0.5945, 0.5599, 0.5066, 0.4404, 0.3679, 0.2952, 0.2276, 0.1686, 0.1200, 0.0821, 0.0539, 0.0340, 0.0207, 0.0120, 0.0067,
			0.0081, 0.0144, 0.0247, 0.0408, 0.0646, 0.0983, 0.1437, 0.2019, 0.2725, 0.3535, 0.4404, 0.5273, 0.6065, 0.6703, 0.7118, 0.7261, 0.7118, 0.6703, 0.6065, 0.5273, 0.4404, 0.3535, 0.2725, 0.2019, 0.1437, 0.0983, 0.0646, 0.0408, 0.0247, 0.0144, 0.0081,
			0.0093, 0.0166, 0.0284, 0.0469, 0.0743, 0.1130, 0.1653, 0.2322, 0.3135, 0.4066, 0.5066, 0.6065, 0.6977, 0.7711, 0.8187, 0.8353, 0.8187, 0.7711, 0.6977, 0.6065, 0.5066, 0.4066, 0.3135, 0.2322, 0.1653, 0.1130, 0.0743, 0.0469, 0.0284, 0.0166, 0.0093,
			0.0103, 0.0183, 0.0314, 0.0518, 0.0821, 0.1249, 0.1827, 0.2567, 0.3465, 0.4493, 0.5599, 0.6703, 0.7711, 0.8521, 0.9048, 0.9231, 0.9048, 0.8521, 0.7711, 0.6703, 0.5599, 0.4493, 0.3465, 0.2567, 0.1827, 0.1249, 0.0821, 0.0518, 0.0314, 0.0183, 0.0103,
			0.0109, 0.0194, 0.0334, 0.0550, 0.0872, 0.1327, 0.1940, 0.2725, 0.3679, 0.4771, 0.5945, 0.7118, 0.8187, 0.9048, 0.9608, 0.9802, 0.9608, 0.9048, 0.8187, 0.7118, 0.5945, 0.4771, 0.3679, 0.2725, 0.1940, 0.1327, 0.0872, 0.0550, 0.0334, 0.0194, 0.0109,
			0.0111, 0.0198, 0.0340, 0.0561, 0.0889, 0.1353, 0.1979, 0.2780, 0.3753, 0.4868, 0.6065, 0.7261, 0.8353, 0.9231, 0.9802, 1.0000, 0.9802, 0.9231, 0.8353, 0.7261, 0.6065, 0.4868, 0.3753, 0.2780, 0.1979, 0.1353, 0.0889, 0.0561, 0.0340, 0.0198, 0.0111,
			0.0109, 0.0194, 0.0334, 0.0550, 0.0872, 0.1327, 0.1940, 0.2725, 0.3679, 0.4771, 0.5945, 0.7118, 0.8187, 0.9048, 0.9608, 0.9802, 0.9608, 0.9048, 0.8187, 0.7118, 0.5945, 0.4771, 0.3679, 0.2725, 0.1940, 0.1327, 0.0872, 0.0550, 0.0334, 0.0194, 0.0109,
			0.0103, 0.0183, 0.0314, 0.0518, 0.0821, 0.1249, 0.1827, 0.2567, 0.3465, 0.4493, 0.5599, 0.6703, 0.7711, 0.8521, 0.9048, 0.9231, 0.9048, 0.8521, 0.7711, 0.6703, 0.5599, 0.4493, 0.3465, 0.2567, 0.1827, 0.1249, 0.0821, 0.0518, 0.0314, 0.0183, 0.0103,
			0.0093, 0.0166, 0.0284, 0.0469, 0.0743, 0.1130, 0.1653, 0.2322, 0.3135, 0.4066, 0.5066, 0.6065, 0.6977, 0.7711, 0.8187, 0.8353, 0.8187, 0.7711, 0.6977, 0.6065, 0.5066, 0.4066, 0.3135, 0.2322, 0.1653, 0.1130, 0.0743, 0.0469, 0.0284, 0.0166, 0.0093,
			0.0081, 0.0144, 0.0247, 0.0408, 0.0646, 0.0983, 0.1437, 0.2019, 0.2725, 0.3535, 0.4404, 0.5273, 0.6065, 0.6703, 0.7118, 0.7261, 0.7118, 0.6703, 0.6065, 0.5273, 0.4404, 0.3535, 0.2725, 0.2019, 0.1437, 0.0983, 0.0646, 0.0408, 0.0247, 0.0144, 0.0081,
			0.0067, 0.0120, 0.0207, 0.0340, 0.0539, 0.0821, 0.1200, 0.1686, 0.2276, 0.2952, 0.3679, 0.4404, 0.5066, 0.5599, 0.5945, 0.6065, 0.5945, 0.5599, 0.5066, 0.4404, 0.3679, 0.2952, 0.2276, 0.1686, 0.1200, 0.0821, 0.0539, 0.0340, 0.0207, 0.0120, 0.0067,
			0.0054, 0.0097, 0.0166, 0.0273, 0.0433, 0.0659, 0.0963, 0.1353, 0.1827, 0.2369, 0.2952, 0.3535, 0.4066, 0.4493, 0.4771, 0.4868, 0.4771, 0.4493, 0.4066, 0.3535, 0.2952, 0.2369, 0.1827, 0.1353, 0.0963, 0.0659, 0.0433, 0.0273, 0.0166, 0.0097, 0.0054,
			0.0042, 0.0074, 0.0128, 0.0211, 0.0334, 0.0508, 0.0743, 0.1044, 0.1409, 0.1827, 0.2276, 0.2725, 0.3135, 0.3465, 0.3679, 0.3753, 0.3679, 0.3465, 0.3135, 0.2725, 0.2276, 0.1827, 0.1409, 0.1044, 0.0743, 0.0508, 0.0334, 0.0211, 0.0128, 0.0074, 0.0042,
			0.0031, 0.0055, 0.0095, 0.0156, 0.0247, 0.0376, 0.0550, 0.0773, 0.1044, 0.1353, 0.1686, 0.2019, 0.2322, 0.2567, 0.2725, 0.2780, 0.2725, 0.2567, 0.2322, 0.2019, 0.1686, 0.1353, 0.1044, 0.0773, 0.0550, 0.0376, 0.0247, 0.0156, 0.0095, 0.0055, 0.0031,
			0.0022, 0.0039, 0.0067, 0.0111, 0.0176, 0.0268, 0.0392, 0.0550, 0.0743, 0.0963, 0.1200, 0.1437, 0.1653, 0.1827, 0.1940, 0.1979, 0.1940, 0.1827, 0.1653, 0.1437, 0.1200, 0.0963, 0.0743, 0.0550, 0.0392, 0.0268, 0.0176, 0.0111, 0.0067, 0.0039, 0.0022,
			0.0015, 0.0027, 0.0046, 0.0076, 0.0120, 0.0183, 0.0268, 0.0376, 0.0508, 0.0659, 0.0821, 0.0983, 0.1130, 0.1249, 0.1327, 0.1353, 0.1327, 0.1249, 0.1130, 0.0983, 0.0821, 0.0659, 0.0508, 0.0376, 0.0268, 0.0183, 0.0120, 0.0076, 0.0046, 0.0027, 0.0015,
			0.0010, 0.0018, 0.0030, 0.0050, 0.0079, 0.0120, 0.0176, 0.0247, 0.0334, 0.0433, 0.0539, 0.0646, 0.0743, 0.0821, 0.0872, 0.0889, 0.0872, 0.0821, 0.0743, 0.0646, 0.0539, 0.0433, 0.0334, 0.0247, 0.0176, 0.0120, 0.0079, 0.0050, 0.0030, 0.0018, 0.0010,
			0.0006, 0.0011, 0.0019, 0.0032, 0.0050, 0.0076, 0.0111, 0.0156, 0.0211, 0.0273, 0.0340, 0.0408, 0.0469, 0.0518, 0.0550, 0.0561, 0.0550, 0.0518, 0.0469, 0.0408, 0.0340, 0.0273, 0.0211, 0.0156, 0.0111, 0.0076, 0.0050, 0.0032, 0.0019, 0.0011, 0.0006,
			0.0004, 0.0007, 0.0012, 0.0019, 0.0030, 0.0046, 0.0067, 0.0095, 0.0128, 0.0166, 0.0207, 0.0247, 0.0284, 0.0314, 0.0334, 0.0340, 0.0334, 0.0314, 0.0284, 0.0247, 0.0207, 0.0166, 0.0128, 0.0095, 0.0067, 0.0046, 0.0030, 0.0019, 0.0012, 0.0007, 0.0004,
			0.0002, 0.0004, 0.0007, 0.0011, 0.0018, 0.0027, 0.0039, 0.0055, 0.0074, 0.0097, 0.0120, 0.0144, 0.0166, 0.0183, 0.0194, 0.0198, 0.0194, 0.0183, 0.0166, 0.0144, 0.0120, 0.0097, 0.0074, 0.0055, 0.0039, 0.0027, 0.0018, 0.0011, 0.0007, 0.0004, 0.0002,
			0.0001, 0.0002, 0.0004, 0.0006, 0.0010, 0.0015, 0.0022, 0.0031, 0.0042, 0.0054, 0.0067, 0.0081, 0.0093, 0.0103, 0.0109, 0.0111, 0.0109, 0.0103, 0.0093, 0.0081, 0.0067, 0.0054, 0.0042, 0.0031, 0.0022, 0.0015, 0.0010, 0.0006, 0.0004, 0.0002, 0.0001
		};

		double blur_weight_sum = 0; // Soma total dos pesos
		for (int i = 0; i < BLUR_SIZE; i++) // Somatório dos pesos
		{
			for (int j = 0; j < BLUR_SIZE; j++)
			{
				blur_weight_sum += blur_distribution[i][j];
			}
		}

		// Cálculo dos pesos
		for (int i = 0; i < BLUR_SIZE; i++)
		{
			for (int j = 0; j < BLUR_SIZE; j++)
			{
				blur_weights[i][j] = blur_distribution[i][j] / blur_weight_sum;
			}
		}
	}
	// / Configuração do efeito blur




	
	// Caminhos dos arquivos
	
	input_file_path  = "..\\files\\breaking_bad_p6.ppm";
	output_file_path = "..\\files\\breaking_bad_p3.ppm";
	
	// / Caminhos dos arquivos





	// Leitura do arquivo
	
	reading_time_start = omp_get_wtime();

	// Abre o arquivo original
    input_file_handle = fopen(input_file_path, "rb");
	output_file_handle = fopen(output_file_path, "w+"); // ?
	
	// Leitura das dimensões da imagem
	{
		char magic_number[3];
		int width, height, depth;
		fscanf(input_file_handle, "%s", magic_number);
		fscanf(input_file_handle, "\n%d %d\n%d\n", &width, &height, &depth);

		image_width  = width;
		image_height = height;
		image_depth  = depth;

		image_pixel_count = image_width * image_height;
	}

	// Declação das matrizes de pixels
	pixel *original_pixels = new pixel[image_width * image_height]; // Imagem original
	pixel *new_pixels = new pixel[image_width * image_height]; // Imagem a ser criada

	// Leitura dos pixels da imagem
	{
		//printf(":: Leitura ::\n");

		int index = 0; // Contador do pixel atual

		for (int i = 0; i < image_height; i++)
		{
			for (int j = 0; j < image_width; j++)
			{
				unsigned char red, green, blue;  // Cores
				fread(&red,   1, 1, input_file_handle);
				fread(&green, 1, 1, input_file_handle);
				fread(&blue,  1, 1, input_file_handle);

				original_pixels[index].red   = red;
				original_pixels[index].green = green;
				original_pixels[index].blue  = blue;
				
				index++;
			}

			//printf("    Leitura %d / %d\n", (i + 1), image_height);
		}

		//printf("// Leitura //\n");
	}

	fclose(input_file_handle);

	reading_time_end = omp_get_wtime();
	reading_time_total = reading_time_end - reading_time_start;

	// / Leitura do arquivo





	// Tratamento paralelo da imagem
	processing_time_start = omp_get_wtime();

	{
		//printf(":: Tratamento ::\n");

		#pragma omp parallel num_threads(thread_count)
		{
			#pragma omp for nowait
			for (int i = 0; i < image_height; i++)
			{
				for (int j = 0; j < image_width; j++)
				{
					double final_red, final_green, final_blue;
					final_red = final_green = final_blue = 0;

					for (int i2 = 0; i2 < BLUR_SIZE; i2++)
					{
						for (int j2 = 0; j2 < BLUR_SIZE; j2++)
						{
							int index = get_pixel_index(
								i + ((-BLUR_SIZE / 2) + i2), 
								j + ((-BLUR_SIZE / 2) + j2), 
								image_width, 
								image_height
							);
						
							pixel p = original_pixels[index];

							final_red   += p.red   * blur_weights[i2][j2];
							final_green += p.green * blur_weights[i2][j2];
							final_blue  += p.blue  * blur_weights[i2][j2];
						}
					}
				
					int index = get_pixel_index(i, j, image_width, image_height);
					new_pixels[index].red   = final_red;
					new_pixels[index].green = final_green;
					new_pixels[index].blue  = final_blue;
				}

				//printf("    Tratamento %d / %d\n", (i + 1), image_height);
			}
		}

		//printf("// Tratamento //\n");
	}

	processing_time_end = omp_get_wtime();
	processing_time_total = processing_time_end - processing_time_start;

	// / Tratamento paralelo da imagem





	// Gravação da imagem nova

	writing_time_start = omp_get_wtime();

	{
		// Criação do arquivo
		//output_file_handle = fopen(output_file_path, "w+");

		// Número mágico do arquivo
		fprintf(output_file_handle, "P3\n");
		
		// Largura, altura e profundidade da imagem
		fprintf(output_file_handle, "%d %d\n%d\n", 
			image_width, image_height, image_depth);

		//printf(":: Gravação ::\n");

		int index = 0; // Contador do pixel atual

		for (int i = 0; i < image_height; i++)
		{
			for (int j = 0; j < image_width; j++)
			{
				pixel p = new_pixels[index];
				
				fprintf(output_file_handle, "%d %d %d ", 
					p.red, 
					p.green, 
					p.blue);

				index++;
			}

			//printf("    Gravação %d / %d\n", (i + 1), image_height);

			fprintf(output_file_handle, "\n");
		}

		//printf("// Gravação //\n");

		fclose(output_file_handle);
	}

	writing_time_end = omp_get_wtime();
	writing_time_total = writing_time_end - writing_time_start;

	// / Gravação da imagem nova





	// Exibe os tempos totais
	total_time = 
		reading_time_total + 
		processing_time_total + 
		writing_time_total;
	
	printf("\nTEMPO DE LEITURA:       %f \n", reading_time_total);	
	printf("\nTEMPO DE PROCESSAMENTO: %f \n", processing_time_total);
	printf("\nTEMPO DE GRAVAÇÃO:      %f \n", writing_time_total);
	printf("\nTEMPO TOTAL:            %f \n", total_time);
	// / Exibe os tempos totais
	


	

	// Interrompe a execução
	getchar();
	getchar();
	getchar();
	getchar();
	getchar();
	getchar();
	getchar();
	getchar();
	getchar();
	getchar();
	// / Interrompe a execução
	

	return 0;
}
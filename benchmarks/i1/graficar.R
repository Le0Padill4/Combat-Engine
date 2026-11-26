args <- commandArgs(trailingOnly = TRUE)
if (length(args) != 1) stop("Uso: Rscript --vanilla graficar.R directorio_resultados")
directorio <- args[[1]]
datos <- read.csv(file.path(directorio, "busqueda_por_id.csv"), stringsAsFactors = FALSE)
columnas <- c("candidato", "escenario", "n", "ns_por_consulta", "bytes_estructura_estimados")
if (!all(columnas %in% names(datos))) stop("CSV sin columnas requeridas")
if (any(!is.finite(datos$ns_por_consulta)) || any(datos$ns_por_consulta <= 0))
    stop("Mediciones de latencia invalidas")
resumen <- aggregate(ns_por_consulta ~ candidato + escenario + n, datos, median)
colores <- c(hash = "#176b87", vector_ordenado = "#b54b27")
etiquetas <- c("Hash propia", "Vector ordenado")
titulos <- c(popular_90_por_ciento = "Consultas concentradas (90%)",
             uniforme_80_aciertos = "Consultas uniformes (80% aciertos)")

abrir <- function(nombre, ancho, alto, formato) {
    archivo <- file.path(directorio, paste0(nombre, ".", formato))
    if (formato == "svg") svg(archivo, width = ancho, height = alto)
    else png(archivo, width = ancho, height = alto, units = "in", res = 150)
}

for (formato in c("svg", "png")) {
abrir("busqueda_por_id", 11, 5, formato)
par(mfrow = c(1, 2), mar = c(4.5, 4.8, 3.5, 1))
for (escenario in unique(resumen$escenario)) {
    panel <- resumen[resumen$escenario == escenario, ]
    plot(range(panel$n), range(panel$ns_por_consulta), type = "n", log = "xy", xaxt = "n",
         xlab = "Perfiles (n)", ylab = "ns por consulta (mediana)",
         main = titulos[[escenario]])
    tamanos <- sort(unique(panel$n))
    axis(1, at = tamanos, labels = format(tamanos, scientific = FALSE, trim = TRUE))
    grid()
    for (candidato in names(colores)) {
        serie <- panel[panel$candidato == candidato, ]
        serie <- serie[order(serie$n), ]
        lines(serie$n, serie$ns_por_consulta, type = "b", pch = 19,
              col = colores[[candidato]], lwd = 2)
    }
    legend("topleft", legend = etiquetas, col = colores, lty = 1,
           pch = 19, bty = "n", cex = 0.8)
}
invisible(dev.off())

memoria <- aggregate(bytes_estructura_estimados ~ candidato + n, datos, median)
abrir("memoria_estructural", 7, 5, formato)
par(mar = c(6, 5, 3.5, 1))
plot(range(memoria$n), range(memoria$bytes_estructura_estimados / 1024^2),
     type = "n", log = "xy", xaxt = "n", xlab = "Perfiles (n)",
     ylab = "MiB estructurales estimados",
     main = "Registro: estimacion parcial de espacio")
tamanos <- sort(unique(memoria$n))
axis(1, at = tamanos, labels = format(tamanos, scientific = FALSE, trim = TRUE))
grid()
for (candidato in names(colores)) {
    serie <- memoria[memoria$candidato == candidato, ]
    serie <- serie[order(serie$n), ]
    lines(serie$n, serie$bytes_estructura_estimados / 1024^2,
          type = "b", pch = 19, col = colores[[candidato]], lwd = 2)
}
legend("topleft", legend = etiquetas, col = colores, lty = 1, pch = 19, bty = "n")
mtext("Excluye buffers dinamicos de texto y metadatos del asignador; no es RSS.",
      side = 1, line = 4.8, cex = 0.68)
invisible(dev.off())
}
write.csv(resumen, file.path(directorio, "medianas.csv"), row.names = FALSE)
cat("Graficos SVG/PNG y medianas generados.\n")

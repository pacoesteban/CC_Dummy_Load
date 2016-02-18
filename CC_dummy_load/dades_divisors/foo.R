plot(mydata2$Vin,(mydata2$Vin), type="o", col="grey")
lines(mydata2$Vin,(mydata2$Div_terra*10), type="o", col="red",pch=22, lty=2)
lines(mydata2$Vin,(mydata2$Div_float*10), type="o", col="blue",pch=22, lty=2)
lines(mydata2$Vin,(mydata2$only_pot*10), type="o", col="green",pch=22, lty=2)

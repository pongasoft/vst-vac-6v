int batchSize = 10

double iZoomFactor = 2.4

int fZoomFactor = (int)(iZoomFactor * batchSize)

int accumulatedZoom = 0;
int accumulatedSamples = 0;
int j = 0;
int totalBatchSize = 0;
for(int i = 0; i < batchSize; i++)
{
  int newAccumulatedZoom = accumulatedZoom + fZoomFactor;
  
  while(newAccumulatedZoom >= batchSize)
  {
    accumulatedSamples++;
    newAccumulatedZoom -= batchSize;
  }
  
  println "${j++} = ${accumulatedSamples} / ${totalBatchSize}"
  totalBatchSize += accumulatedSamples;
  accumulatedSamples = 0;
  
  accumulatedZoom = newAccumulatedZoom;
}

println "fZoomFactor=${fZoomFactor} / totalBatchSize=${totalBatchSize}"

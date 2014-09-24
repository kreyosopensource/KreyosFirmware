#include "grlib.h"

#define abs(A) ((A)>0?(A):-(A))
#define signum(A) ((A)==0?0:((A)>0?1:-1))

static void fillFlatSideTriangleInt(const tContext *pContext, long lXA, long lYA,
                                             long lXB, long lYB,
                                             long lXC, long lYC)
{
  long lXStart, lYStart, lXEnd, lYEnd;
  lXStart = lXEnd = lXA;
  lYStart = lYEnd = lYA;

  int changed1 = 0;
  int changed2 = 0;

  int dx1 = abs(lXB - lXA);
  int dy1 = abs(lYB - lYA);

  int dx2 = abs(lXC - lXA);
  int dy2 = abs(lYC - lYA);

  int signx1 = signum(lXB - lXA);
  int signx2 = signum(lXC - lXA);
  
  int signy1 = signum(lYB - lYA);
  int signy2 = signum(lYC - lYA);

  if (dy1 > dx1)
  {   // swap values
    int tmp = dx1;
    dx1 = dy1;
    dy1 = tmp;
    changed1 = 1;
  }

  if (dy2 > dx2)
  {   // swap values
    int tmp = dx2;
    dx2 = dy2;
    dy2 = tmp;
    changed2 = 1;
  }

  int e1 = 2 * dy1 - dx1;
  int e2 = 2 * dy2 - dx2;

  for (int i = 0; i <= dx1; i++)
  {
    GrLineDraw(pContext, lXStart, lYStart, lXEnd, lYEnd);
    
    while (e1 >= 0)
    {
        if (changed1)
            lXStart += signx1;
        else
            lYStart += signy1;
        e1 = e1 - 2 * dx1;
    }
    
    if (changed1)
        lYStart += signy1;
    else
        lXStart += signx1;  

    e1 = e1 + 2 * dy1;
    
    /* here we rendered the next point on line 1 so follow now line 2
     * until we are on the same y-value as line 1.
     */
    while (lYEnd != lYStart)
    {
        while (e2 >= 0)
        {
            if (changed2)
                lXEnd += signx2;
            else
                lYEnd += signy2;
            e2 = e2 - 2 * dx2;
        }

        if (changed2)
            lYEnd += signy2;
        else
            lXEnd += signx2;

        e2 = e2 + 2 * dy2;
    }
  }

}

void GrTriagleFill(const tContext *pContext, long lXA, long lYA,
                                             long lXB, long lYB,
                                             long lXC, long lYC)
{
  long tmp;
  // sort A, B , C in Y axis in order
  if (lYA > lYB)
  {
    tmp = lYA; lYA = lYB; lYB = tmp;
    tmp = lXA; lXA = lXB; lXB = tmp;
  }

  if (lYB > lYC)
  {
    tmp = lYB; lYB = lYC; lYC = tmp;
    tmp = lXB; lXB = lXC; lXC = tmp;

    if (lYA > lYB)
    {
      tmp = lYA; lYA = lYB; lYB = tmp;
      tmp = lXA; lXA = lXB; lXB = tmp;
    }
  }

  assert(lYA <= lYB && lYB <= lYC);

    /* here we know that v1.y <= v2.y <= v3.y */
    /* check for trivial case of bottom-flat triangle */
    if (lYB == lYC)
    {
        fillFlatSideTriangleInt(pContext, lXA, lYA, lXB, lYB, lXC, lYC);
    }
    /* check for trivial case of top-flat triangle */
    else if (lYA == lYB)
    {
        fillFlatSideTriangleInt(pContext, lXC, lYC, lXA, lYA, lXB, lYB);
    } 
    else
    {
        /* general case - split the triangle in a topflat and bottom-flat one */
        long lXT = lXA + (lYB - lYA)  * (lXC - lXA) / (lYC - lYA);
        fillFlatSideTriangleInt(pContext, lXA, lYA, lXB, lYB, lXT, lYB);
        fillFlatSideTriangleInt(pContext, lXC, lYC, lXB, lYB, lXT, lYB);
    }
}


void GrTriagleDraw(const tContext *pContext, long lXA, long lYA,
                                             long lXB, long lYB,
                                             long lXC, long lYC)
{
  GrLineDraw(pContext, lXA, lYA, lXB, lYB);
  GrLineDraw(pContext, lXB, lYB, lXC, lYC);
  GrLineDraw(pContext, lXA, lYA, lXC, lYC);
}
#include "n25493/m25493.h"
QVector<double> m25493::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "k29830/m29830.h"
QVector<double> m29830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

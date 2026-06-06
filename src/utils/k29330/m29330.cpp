#include "k29330/m29330.h"
QVector<double> m29330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

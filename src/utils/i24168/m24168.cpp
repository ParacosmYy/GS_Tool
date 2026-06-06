#include "i24168/m24168.h"
QVector<double> m24168::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

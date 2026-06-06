#include "i20608/m20608.h"
QVector<double> m20608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

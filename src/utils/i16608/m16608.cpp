#include "i16608/m16608.h"
QVector<double> m16608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

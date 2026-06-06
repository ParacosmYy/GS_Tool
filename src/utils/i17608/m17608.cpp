#include "i17608/m17608.h"
QVector<double> m17608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

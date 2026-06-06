#include "i28608/m28608.h"
QVector<double> m28608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

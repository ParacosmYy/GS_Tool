#include "i9608/m9608.h"
QVector<double> m9608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "m18892/m18892.h"
QVector<double> m18892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

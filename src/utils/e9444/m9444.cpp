#include "e9444/m9444.h"
QVector<double> m9444::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

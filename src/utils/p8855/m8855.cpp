#include "p8855/m8855.h"
QVector<double> m8855::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

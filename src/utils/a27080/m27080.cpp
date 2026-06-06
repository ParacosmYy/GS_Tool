#include "a27080/m27080.h"
QVector<double> m27080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

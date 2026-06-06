#include "i25288/m25288.h"
QVector<double> m25288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

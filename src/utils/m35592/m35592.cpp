#include "m35592/m35592.h"
QVector<double> m35592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

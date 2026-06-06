#include "a35800/m35800.h"
QVector<double> m35800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "a35500/m35500.h"
QVector<double> m35500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

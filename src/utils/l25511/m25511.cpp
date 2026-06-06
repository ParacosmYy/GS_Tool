#include "l25511/m25511.h"
QVector<double> m25511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

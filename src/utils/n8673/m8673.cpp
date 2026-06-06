#include "n8673/m8673.h"
QVector<double> m8673::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

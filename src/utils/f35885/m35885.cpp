#include "f35885/m35885.h"
QVector<double> m35885::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

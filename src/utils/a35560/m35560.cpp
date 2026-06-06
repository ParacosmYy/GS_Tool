#include "a35560/m35560.h"
QVector<double> m35560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

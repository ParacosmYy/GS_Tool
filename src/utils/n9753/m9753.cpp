#include "n9753/m9753.h"
QVector<double> m9753::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

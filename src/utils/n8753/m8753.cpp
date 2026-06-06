#include "n8753/m8753.h"
QVector<double> m8753::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

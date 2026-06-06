#include "k27650/m27650.h"
QVector<double> m27650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

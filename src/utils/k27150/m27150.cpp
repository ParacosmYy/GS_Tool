#include "k27150/m27150.h"
QVector<double> m27150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

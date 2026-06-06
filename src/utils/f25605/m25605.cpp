#include "f25605/m25605.h"
QVector<double> m25605::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

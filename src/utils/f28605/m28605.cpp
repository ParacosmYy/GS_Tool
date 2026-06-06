#include "f28605/m28605.h"
QVector<double> m28605::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

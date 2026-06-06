#include "a37280/m37280.h"
QVector<double> m37280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "a27280/m27280.h"
QVector<double> m27280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

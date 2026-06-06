#include "o8154/m8154.h"
QVector<double> m8154::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

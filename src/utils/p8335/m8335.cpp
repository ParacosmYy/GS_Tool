#include "p8335/m8335.h"
QVector<double> m8335::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

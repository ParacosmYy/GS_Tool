#include "p8835/m8835.h"
QVector<double> m8835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

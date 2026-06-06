#include "f30685/m30685.h"
QVector<double> m30685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "e8084/m8084.h"
QVector<double> m8084::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "e8184/m8184.h"
QVector<double> m8184::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

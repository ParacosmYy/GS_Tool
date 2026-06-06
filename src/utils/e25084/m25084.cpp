#include "e25084/m25084.h"
QVector<double> m25084::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

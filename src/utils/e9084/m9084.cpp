#include "e9084/m9084.h"
QVector<double> m9084::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

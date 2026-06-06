#include "k31550/m31550.h"
QVector<double> m31550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

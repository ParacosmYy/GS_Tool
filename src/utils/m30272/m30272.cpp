#include "m30272/m30272.h"
QVector<double> m30272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

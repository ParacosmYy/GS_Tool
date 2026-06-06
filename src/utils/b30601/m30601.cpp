#include "b30601/m30601.h"
QVector<double> m30601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "m30632/m30632.h"
QVector<double> m30632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

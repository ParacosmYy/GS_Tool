#include "m21632/m21632.h"
QVector<double> m21632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "k31810/m31810.h"
QVector<double> m31810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

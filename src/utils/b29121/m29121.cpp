#include "b29121/m29121.h"
QVector<double> m29121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

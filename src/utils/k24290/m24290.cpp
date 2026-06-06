#include "k24290/m24290.h"
QVector<double> m24290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

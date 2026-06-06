#include "g30706/m30706.h"
QVector<double> m30706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

#include "o32514/m32514.h"
QVector<double> m32514::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

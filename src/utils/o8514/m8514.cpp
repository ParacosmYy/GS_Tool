#include "o8514/m8514.h"
QVector<double> m8514::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

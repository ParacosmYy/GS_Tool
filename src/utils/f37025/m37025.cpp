#include "f37025/m37025.h"
QVector<double> m37025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

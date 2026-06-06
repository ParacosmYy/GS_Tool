#include "k28290/m28290.h"
QVector<double> m28290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }

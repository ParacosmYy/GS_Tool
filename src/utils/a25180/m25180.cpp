#include "a25180/m25180.h"
QVector<double> m25180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
